<script lang="ts" module>
	import type { Component } from 'svelte';

	export type Feature = {
		title: string;
		body: string;
		href?: string;
		Icon: Component<{ size?: number; strokeWidth?: number; class?: string }>;
		cta?: string;
		soon?: boolean;
	};
</script>

<script lang="ts">
	import ArrowRightIcon from 'lucide-svelte/icons/arrow-right';

	let { features, soonBadge = 'Soon' }: { features: Feature[]; soonBadge?: string } = $props();
</script>

<div class="grid gap-4 sm:grid-cols-2">
	{#each features as p (p.title)}
		<a
			href={p.href}
			class={`group flex flex-col rounded-2xl border border-border bg-card p-7 transition-colors md:p-9 ${p.href ? 'hover:border-primary/30 hover:bg-primary/3' : 'cursor-default'}`}
			target={p.href?.startsWith('http') && p.href.includes('://') ? '_blank' : undefined}
		>
			<div class="mb-5 flex h-10 w-10 items-center justify-center rounded-xl bg-primary/10 text-primary">
				<p.Icon size={20} strokeWidth={1.5} />
			</div>
			<div class="flex-1">
				<div class="flex items-center gap-2">
					<h3 class="font-semibold">{p.title}</h3>
					{#if p.soon}
						<span class="rounded-full bg-muted px-2 py-0.5 text-[10px] font-semibold tracking-wide text-muted-foreground uppercase">
							{soonBadge}
						</span>
					{/if}
				</div>
				<p class="mt-1 text-sm leading-relaxed text-muted-foreground">{p.body}</p>
			</div>
			{#if p.href}
				<div class="mt-6 flex items-center gap-1 text-sm font-medium text-primary">
					{p.cta}
					<ArrowRightIcon size={14} class="transition-transform group-hover:translate-x-1" />
				</div>
			{/if}
		</a>
	{/each}
</div>
