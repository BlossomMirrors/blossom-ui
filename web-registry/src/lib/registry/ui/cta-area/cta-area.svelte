<script lang="ts">
	import { Button } from "$lib/registry/ui/button/index.js";

	let {
		title,
		body,
		button1Text,
		button1Href = '#',
		button2Text,
		button2Href,
	}: {
		title: string;
		body: string;
		button1Text: string;
		button1Href?: string;
		button2Text?: string;
		button2Href?: string;
	} = $props();

	function isExternal(href: string) {
		return href.startsWith('http');
	}
</script>

<div class="relative overflow-hidden rounded-3xl bg-linear-to-br from-primary/10 via-primary/5 to-transparent p-10 md:p-16">
	<div class="pointer-events-none absolute -top-16 -right-16 h-64 w-64 rounded-full bg-primary/8 blur-3xl"></div>
	<div class="relative max-w-2xl">
		<h2 class="font-serif text-4xl leading-tight md:text-5xl">{title}</h2>
		<p class="mt-5 text-lg leading-relaxed text-muted-foreground">{body}</p>
		<div class="mt-8 flex flex-wrap gap-3">
			<a
				href={button1Href}
				target={isExternal(button1Href) ? '_blank' : undefined}
				rel={isExternal(button1Href) ? 'noreferrer' : undefined}
			>
				<Button variant="primary">{button1Text}</Button>
			</a>
			{#if button2Text && button2Href}
				<a
					href={button2Href}
					target={isExternal(button2Href) ? '_blank' : undefined}
					rel={isExternal(button2Href) ? 'noreferrer' : undefined}
				>
					<Button>{button2Text}</Button>
				</a>
			{/if}
		</div>
	</div>
</div>
